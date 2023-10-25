#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <fstream>
#include <ctime>
#include <string>

class Session : public boost::enable_shared_from_this<Session> {
public:
    Session(boost::asio::io_service& io_service) : socket_(io_service), timer_(io_service) {
        last_active_time_ = std::time(nullptr);
    }

    void start() {
        // Configurar um temporizador para desconectar clientes inativos
        startTimeoutTimer();

        boost::asio::async_read(socket_, boost::asio::buffer(data_, MAX_FILE_SIZE),
            boost::bind(&Session::handleRead, shared_from_this(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    boost::asio::ip::tcp::socket& socket() {
        return socket_;
    }

    void handleRead(const boost::system::error_code& error, size_t bytes_transferred) {
        if (!error) {
            // Salvar os dados em um novo arquivo
            std::string filename = FILENAME + std::to_string(fileCounter_++) + ".txt";
            std::ofstream file(filename, std::ios::app);
            file.write(data_.data(), bytes_transferred);
            file.close();

            // Reiniciar o temporizador
            resetTimeoutTimer();

            boost::asio::async_read(socket_, boost::asio::buffer(data_, MAX_FILE_SIZE),
                boost::bind(&Session::handleRead, shared_from_this(),
                boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        } else {
            // Cliente desconectado ou erro de leitura
            if (error == boost::asio::error::eof) {
                std::cout << "Cliente desconectado" << std::endl;
            } else {
                std::cerr << "Erro: " << error.message() << std::endl;
            }
        }
    }
    
    //carrega as configurações do arquivo config.ini porem se os valores não forem encontrados atribui valores padrão
    static void loadConfig() {  
        boost::property_tree::ptree config;
        boost::property_tree::ini_parser::read_ini("config.ini", config);
        
        MAX_FILE_SIZE = config.get<int>("MAX_FILE_SIZE", 1024);
        TIMEOUT_TIMER = config.get<int>("TIMEOUT_TIMER", 60);
        PORT = config.get<int>("PORT", 12345);
        FILENAME = config.get<std::string>("FILENAME", "FILENAME");
    }

private:
    void startTimeoutTimer() {
        timer_.expires_from_now(boost::posix_time::seconds(TIMEOUT_TIMER));
        timer_.async_wait(boost::bind(&Session::handleTimeout, shared_from_this(), _1));
    }

    void resetTimeoutTimer() {
        timer_.cancel();
        startTimeoutTimer();
    }

    void handleTimeout(const boost::system::error_code& error) {
        if (!error) {
            std::cout << "Cliente desconectado devido a inatividade." << std::endl;
            socket_.close();
        }
    }

    boost::asio::ip::tcp::socket socket_;
    boost::asio::deadline_timer timer_;
    std::array<char, MAX_FILE_SIZE> data_;
    std::time_t last_active_time_;
    static int fileCounter_;
    static int MAX_FILE_SIZE;
    static int TIMEOUT_TIMER;
    static int PORT;
    static std::string FILENAME;
};

int Session::fileCounter_ = 1;  // Inicializa o contador de arquivo
int Session::MAX_FILE_SIZE; // Remove a inicialização fixa
int Session::TIMEOUT_TIMER; // Remove a inicialização fixa
int Session::PORT; // Remove a inicialização fixa
std::string Session::FILENAME; // Remove a inicialização fixa

class Server {
public:
    Server(boost::asio::io_service& io_service) : acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT)) {
        startAccept();
    }

    void startAccept() {
        Session::loadConfig();  // Carregar configurações antes de aceitar conexões

        Session::pointer new_session = Session::create(acceptor_.get_io_service());

        acceptor_.async_accept(new_session->socket(),
            boost::bind(&Server::handleAccept, this, new_session,
            boost::asio::placeholders::error));
    }

    void handleAccept(Session::pointer session, const boost::system::error_code& error) {
        if (!error) {
            session->start();
        }

        startAccept();
    }

private:
    boost::asio::ip::tcp::acceptor acceptor_;
};

int main() {
    boost::asio::io_service io_service;
    Server server(io_service);
    io_service.run();
    return 0;
}
