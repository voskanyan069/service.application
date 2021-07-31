#ifndef APPLICATION_H
#define APPLICATION_H

#define BOOST_LIB_DIAGNOSTIC

#include <iostream>
#include <boost/application.hpp>
#include <boost/program_options.hpp>
#include "os.h"

#if OS == WIN32 || OS == WIN64
#include <tchar.h>
#include "setup.h"
#endif

namespace po = boost::program_options;

/*
 * This class was for creating, installing and uninstalling service
 */
class service
{
public:
	service(boost::application::context& context)
		: context_(context)
	{
	}

	/*
	 * Overriding when include
	 */
	void work();

	/*
	 * Install the service
	 */
	void install(std::string name)
	{
#if OS == LINUX
		int resid;
		const char* path_str = path.c_str();
		std::string command("sudo cp ");
		command.append(path_str);
		command.append(" /etc/init.d/");
		command.append(name.c_str());
		resid = std::system(command.c_str()); // copy bin file to /etc/init.d/
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
#elif OS == WIN32 || OS == WIN64
		std::string path_str = path.string();
		std::wstring wide_name= std::wstring(name.begin(), name.end());
		std::wstring wide_path = std::wstring(path_str.begin(), path_str.end());
		const wchar_t* svname = wide_name.c_str();
		const wchar_t* svpath = wide_path.c_str();
		install_windows_service(svname, svpath);
#endif
	}

	/*
	 * Uninstall the service
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
		std::wstring wide_string = std::wstring(name.begin(), name.end());
		const wchar_t* svname = wide_string.c_str();
		uninstall_windows_service(svname);
#endif
	}

	/*
	 * Calling other methods from command-line arguments
	 */
	bool setup()
	{
		po::options_description desc("Allowed options");
		desc.add_options()
			("install,i", "install service")
			("uninstall,u", "uninstall service")
			("name,n", po::value<std::string>()->default_value("server"),
			 "name of the service");

		po::variables_map vm;
		po::store(po::parse_command_line(args->argc(), args->argv(), desc), vm);
		po::notify(vm);

		std::string name = vm["name"].as<std::string>(); // -n, --name

		if (vm.count("install")) // -i, --install
		{
			install(name);
			return 1;
		}

		if (vm.count("uninstall")) // -u, --uninstall
		{
			uninstall(name);
			return 1;
		}
		return 0;
	}

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
	 * On service stop [Linux/Windows]
	 */
	bool stop()
	{
		return true;
	}

	/*
	 * On service pause [Windows]
	 */
	bool pause()
	{
		return true;
	}

	/*
	 * On service resume [Windows]
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
 * Application class for creating service apps
 */
class application
{
public:
	application()
	{
	}

	/*
	 * Create service
	 */
#if OS == WIN32 || OS == WIN64
	int start(int argc, _TCHAR* argv[])
#elif OS == LINUX
	int start(int argc, char* argv[])
#endif
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
#endif
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
#endif
			res = boost::application::launch<
				boost::application::common>(app, app_context, ec);
		}

		return res;
	}
};

#endif // APPLICATION_H
