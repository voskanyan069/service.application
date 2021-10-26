#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#define BOOST_LIB_DIAGNOSTIC

#include <iostream>
#include <boost/application/application.hpp>
#include <boost/program_options.hpp>
#include <boost/application/setup/os.hpp>

#if defined(BOOST_APPLICATION_TCHAR)
#include <tchar.h>
#endif // BOOST_APPLICATION_TCHAR

#if OS == WIN32 || OS == WIN64
#include <boost/application/setup/setup.hpp>
#endif // OS == WIN32 || OS == WIN64

namespace po = boost::program_options;

/*
 * brief This class was for creating, installing and uninstalling service
 */
class service
{
public:
	/*
	 * brief Creates a service application
	 * param context The service application context
	 */
	service(boost::application::context& context)
		: context_(context)
	{
	}

	/*
	 * brief Main service function
	 * note Overriding when include
	 */
	void work();

	/*
	 * brief Install the service
	 * param name Service name to isntall
	 * param mode Service install mode [auto/manual]
	 */
	void install(std::string name, std::string mode)
	{
#if OS == LINUX
		if (mode == "auto")
		{
			int resid;
			const char* path_str = path.c_str();
			std::string command("sudo cp ");
			command.append(path_str);
			command.append(" /etc/init.d/");
			command.append(name.c_str());
			resid = std::system(command.c_str()); // copy bin file /etc/init.d
			if (resid != 0)
			{
				std::cout << " [error] " << resid << std::endl;
			}
			command = "update-rc.d ";
			command.append(name.c_str());
			command.append(" defaults");
			resid = std::system(command.c_str()); // update-rc.d <NAME> defaults
			if (resid == 0) // command returned successfully
			{
				std::cout << "Daemon successfully installed" << std::endl;
			}
			else // command reteurned some error
			{
				std::cout << " [error] " << resid << std::endl;
			}
		}
#elif OS == WIN32 || OS == WIN64
		std::string path_str = path.string();
		std::wstring wide_name = std::wstring(name.begin(), name.end());
		std::wstring wide_path = std::wstring(path_str.begin(), path_str.end());
		const wchar_t* svcname = wide_name.c_str();
		const wchar_t* svcpath = wide_path.c_str();
		
		install_windows_service(svcname, mode, svcpath);
#endif // OS == LINUX ? OS == WIN32 || OS == WIN64
	}

	/*
	 * brief Uninstall the service
	 * param name Service name
	 */
	void uninstall(std::string name)
	{
#if OS == LINUX
		int resid;
		std::string command("sudo rm -f /etc/init.d/");
		command.append(name.c_str());
		resid = std::system(command.c_str()); // remove /etc/init.d/<NAME>
		if (resid == 0) // command returned successfully
		{
			std::cout << "Daemon successfully deleted" << std::endl;
		}
		else // command returned some error
		{
			std::cout << " [error] " << resid << std::endl;
		}
#elif OS == WIN32 || OS == WIN64
		std::wstring wide_name = std::wstring(name.begin(), name.end());
		const wchar_t* svcname = wide_name.c_str();
		uninstall_windows_service(svcname);
#endif // OS == LINUX ? OS == WIN32 || OS == WIN64
	}

	/*
	 * brief Calling methods from command-line arguments
	 * return Returning code of executated \
	 * 	[ \
	 * 	 1 - called any command-line arg, \
	 * 	 0 - non arguments called, service runned \
	 *  ]
	 */
	bool setup()
	{
		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "display this help message")
			("install,i", "install service")
			("uninstall,u", "uninstall service")
			("name,n", po::value<std::string>()->default_value("my_service"),
			 "name of the service")
			("mode,m", po::value<std::string>()->default_value("manual"),
			 "service mode [auto, manual*]")
			;

		po::variables_map vm;
		po::store(po::parse_command_line(args->argc(), args->argv(), desc), vm);
		po::notify(vm);

		std::string name = vm["name"].as<std::string>(); // -n, --name
		std::string mode = vm["mode"].as<std::string>(); // -m, --mode

		if (vm.count("help")) // -h, --help
		{
			std::cout << desc << std::endl;
			return 1;
		}

		if (vm.count("install")) // -i, --install
		{
			install(name, mode);
			return 1;
		}

		if (vm.count("uninstall")) // -u, --uninstall
		{
			uninstall(name);
			return 1;
		}
		return 0;
	}

	/*
	 * brief Start method
	 * return Code of runned with command-line arguments or not
	 */
	int operator()()
	{
		args = context_.find<boost::application::args>(); // command-line args
		path = boost::filesystem::system_complete(args->argv()[0]); // exe path

		if (setup()) // called install or uninstall
		{
			return 1;
		}

		// Launch new background thread and call service::work method
		boost::thread thread(&service::work, this);
		context_.find<boost::application
			::wait_for_termination_request>()->wait();

		return 0;
	}

	/*
	 * brief On service stop [Linux/Windows]
	 * return Return true to stop
	 */
	bool stop()
	{
		return true;
	}

	/*
	 * brief On service pause [Windows]
	 * return Return true to pause
	 */
	bool pause()
	{
		return true;
	}

	/*
	 * brief On service resume [Windows]
	 * return Return true to resume
	 */
	bool resume()
	{
		return true;
	}

private:
	boost::application::context& context_;
	std::shared_ptr<boost::application::args> args;
	boost::filesystem::path path;
};

/*
 * brief Application class for creating service apps
 */
class application
{
public:
	/*
	 * brief Creates an application
	 */
	application()
	{
	}

	/*
	 * brief Create service
	 * return Error code
	 */
#if defined(BOOST_APPLICATION_TCHAR) || OS == WIN32 || OS == WIN64
	int start(int argc, _TCHAR* argv[])
#elif OS == LINUX
	int start(int argc, char* argv[])
#endif // BOOST_APPLICATION_TCHAR || OS == WIN32 || OS == WIN64 ? OS == LINUX
	{
		boost::application::context app_context;
		boost::application::auto_handler<service> app(app_context);
		app_context.insert<boost::application::args>(
			std::make_shared<boost::application::args>(argc, argv));
		boost::system::error_code ec;
		int res;

		if (argc == 1) // runned without arguments
		{
			res = boost::application::launch<
#if OS == LINUX
				boost::application::server // launch app in background
#elif OS == WIN32 || OS == WIN64
				boost::application::common // launch app in foreground
#endif // OS == LINUX ? OS == WIN32 || OS == WIN64
				>(app, app_context, ec);
		}
		else
		{
#if OS == LINUX
			if (getuid()) // runned as sudo
			{
				std::cout << "Please run as root" << std::endl;
				return 1;
			}
#endif // OS == LINUX
			res = boost::application::launch<
				boost::application::common>(app, app_context, ec);
		}

		return res;
	}
};

#endif // APPLICATION_HPP
