#ifndef COOKIE_HPP
# define COOKIE_HPP

// #include "CGI.hpp"
// #include "Request.hpp"
# include "map"

// typedef struct t_cookie
// {
// 	std::string uri;
// 	std::string script;
// 	std::string ID;
// } s_cookie;

class Cookie
{
	public:
	//check if cookie is present in http request header
	bool	check_cookie(std::map<std::string, std::string> &headers)
	{
		std::map<std::string, std::string>::iterator	it;
		for (it = headers.begin(); it != headers.end(); ++it)
		{
			if (it->first == "Cookie")
			{
				size_t pos = (it->second).find('=');
				if ((it->second).substr(pos + 1) == this->ID)
					return true;
			}
				// std::cout << "cookie found" << std::endl;
		}
		if (it == headers.end())
			return false;
		return false;
	}

	//if no cookie, create cookie
	//based on? username and age? these values are set in browser
	//once user clicks submit

	//cookie will be set in CGI script. i have to save this as a environment variable
	
	// void	save_cookie(const std::string &value)
	// {
	// 	s_cookie	new_cookie;
	// 	new_cookie.query = value;
	// 	size_t pos = new_cookie.query.find('+');
	// 	new_cookie.name = new_cookie.query.substr(0, pos);
	// 	new_cookie.age = new_cookie.query.substr(pos + 1);
	// 	_cookie.push_back(new_cookie);
	// }
	
	// if cookie exists, check with existing cookie vector, if match then ?
	// do i need this? it will always match. because browser sends cookie only because server creates it
	// void	set_cookie(const std::string &value)
	// {
	// 	size_t pos = value.find(';');
	// 	std::string v_name = value.substr(0, pos);
	// 	std::string v_age = value.substr(pos + 1);
	// 	std::vector<s_cookie>::iterator it;
	// }
	// void	set_cookie(std::stringstream &ss, std::string &ip)
	// {
	// 	ss << std::string("Set-Cookie: ID=") << ip << "\r\n";
	// }

	// void	set_name(std::string &value)
	// {
	// 	cookie.name = value;
	// }
	
	// void	set_age(std::string &value)
	// {
	// 	cookie.age = value;
	// }
	
	void	set_cookie(std::string &req_uri, std::string &req_script, std::string &client_ip)
	{
		if (req_uri.find("icon") != std::string::npos)
			return ;
		this->ID = client_ip;
		this->uri = req_uri;
		this->script = req_script;
	}

	std::string	get_uri(void)
	{
		return (uri);
	}
	
	std::string	get_script(void)
	{
		return (script);
	}

	private:
		//std::vector<s_cookie>	_cookie;
		std::string uri;
		std::string script;
		std::string ID;
		// s_cookie				_cookie;
};

#endif