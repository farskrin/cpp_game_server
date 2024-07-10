#include "sdk.h"

#include "magic_defs.h"
//
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>

#include <iostream>
#include <thread>
#include <mutex>

#include "application.h"
#include "request_handler.h"
#include "logger.h"

using namespace std::literals;
using namespace json_loader;
namespace net = boost::asio;
namespace sys = boost::system;

namespace {

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

//------------------------
    class Ticker : public std::enable_shared_from_this<Ticker> {
    public:
        using Strand = net::strand<net::io_context::executor_type>;
        using Handler = std::function<void(std::chrono::milliseconds delta)>;

        // Функция handler будет вызываться внутри strand с интервалом period
        Ticker(Strand strand, std::chrono::milliseconds period, Handler handler)
                : strand_{strand}
                , period_{period}
                , handler_{std::move(handler)} {
        }

        void Start() {
            net::dispatch(strand_, [self = shared_from_this(), this] {
                last_tick_ = Clock::now();
                self->ScheduleTick();
            });
        }

    private:
        void ScheduleTick() {
            assert(strand_.running_in_this_thread());
            timer_.expires_after(period_);
            timer_.async_wait([self = shared_from_this()](sys::error_code ec) {
                self->OnTick(ec);
            });
        }

        void OnTick(sys::error_code ec) {
            using namespace std::chrono;
            assert(strand_.running_in_this_thread());

            if (!ec) {
                auto this_tick = Clock::now();
                auto delta = duration_cast<milliseconds>(this_tick - last_tick_);
                last_tick_ = this_tick;
                try {
                    handler_(delta);
                } catch (...) {
                }
                ScheduleTick();
            }
        }

        using Clock = std::chrono::steady_clock;

        Strand strand_;
        std::chrono::milliseconds period_;
        net::steady_timer timer_{strand_};
        Handler handler_;
        std::chrono::steady_clock::time_point last_tick_;
    };

//------------------------
    constexpr const char DB_URL_ENV_NAME[]{"GAME_DB_URL"};
    //GAME_DB_URL=postgres://postgres:Passwd@localhost:30432/game_db1

    app::AppConfig GetConfigFromEnv() {
        app::AppConfig config;
        if (const auto* url = std::getenv(DB_URL_ENV_NAME)) {
            config.db_url = url;
        } else {
            throw std::runtime_error(DB_URL_ENV_NAME + " environment variable not found"s);
        }
        return config;
    }

}  // namespace

namespace {
    using namespace std::literals;
    namespace logging = boost::log;
    namespace keywords = boost::log::keywords;
    namespace sinks = boost::log::sinks;
    namespace json = boost::json;
    namespace sys = boost::system;

    BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
    BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", json::value)

    void LogFormatter(logging::record_view const &rec, logging::formatting_ostream &strm) {

        auto ts = *rec[timestamp];
        json::object jobject;
        jobject.emplace("timestamp", to_iso_extended_string(ts));
        jobject.emplace("data", *rec[additional_data]);
        jobject.emplace("message", *rec[logging::expressions::smessage]);
        std::string str = json::serialize(jobject);
        strm << str;
    }

    void SetupLogger() {
        logging::add_common_attributes();
        logging::add_console_log(
                std::cout,
                keywords::format = &LogFormatter,
                keywords::auto_flush = true
        );
    }
}


int main(int argc, const char* argv[]) {
    try {

        // 1. Загружаем карту из файла и построить модель игры
        auto args = options::ParseCommandLine(argc, argv);
        if(!args.has_value()){
            throw std::runtime_error("Can't parsing program options"s);
            return EXIT_FAILURE;
        }

        app::Application app(*args, GetConfigFromEnv());
        app.RestoreBackUpData();

        // 2. Инициализируем io_context
        const unsigned num_threads = 1; //std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
		// Подписываемся на сигналы и при их получении завершаем работу сервера
		net::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
			if (!ec) {
				ioc.stop();
			}
		});

        // strand для выполнения запросов к API
        auto api_strand = net::make_strand(ioc);

        if(args->tick_period){
            auto period = static_cast<std::chrono::milliseconds>(args->tick_period);
            // Настраиваем вызов метода Application::Tick каждые 50 миллисекунд внутри strand
            auto ticker = std::make_shared<Ticker>(api_strand, period,
                                                   [&app](std::chrono::milliseconds delta) { app.Tick(delta); }
            );
            ticker->Start();
        }

        const auto address = net::ip::make_address(ServerParam::ADDR);
        constexpr net::ip::port_type port = ServerParam::PORT;

        auto endpoint = net::ip::tcp::endpoint{address, port};

        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        //http_handler::RequestHandler handler{game, doc_root, api_strand};
        auto handler = std::make_shared<http_handler::RequestHandler>(app, api_strand);

        // Оборачиваем его в логирующий декоратор
        http_handler::LoggingRequestHandler logging_handler{
                [handler, endpoint]( auto&& req, auto&& send) {
                    // Обрабатываем запрос
                    (*handler)(endpoint, std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
                }};

        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        http_server::ServeHttp(ioc, {address, port}, logging_handler);

        SetupLogger();
        json::value start_data{{"port"s, ServerParam::PORT}, {"address", ServerParam::ADDR}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, start_data) << "server started"sv;
        //std::cerr << ServerMessage::START << std::endl;

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });

        app.BackUpData();

        //TODO: finish
        json::value finish_data{{"code"s, 0}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, finish_data) << "server exited"sv;
        //std::cerr << ServerMessage::EXIT << std::endl;

    } catch (const std::exception& ex) {
        json::value finish_data{{"code"s, 1}, {"exception", ex.what()}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, finish_data) << "server exited"sv;

        return EXIT_FAILURE;
    }


}
