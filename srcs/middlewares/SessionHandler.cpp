#include "SessionHandler.hpp"

void SessionHandler::execute(Request &req, Response &res)
{
	current_session_ID = extract_cookie(req._headers["Cookie"], "ID");
	if (current_session_ID.empty())
	{
		current_session_ID = to_string(rand());
		res.set_header("Set-Cookie", "ID=" + current_session_ID);
	}
	Middleware::execute(req, res);
}

const std::string &SessionHandler::get_session()
{
	return _session[current_session_ID];
}

const std::string &SessionHandler::get_session_id() const
{
	return current_session_ID;
}

std::string SessionHandler::extract_cookie(const std::string &cookies, const std::string &cookie_name)
{
	std::size_t begin = cookies.find(cookie_name + "=");
	if (begin == std::string::npos)
		return std::string();
	begin =+ cookie_name.size() + 1;
	std::size_t end = cookies.find(';', begin);
	const std::string cookie_value = cookies.substr(begin, end - begin);
	return cookie_value;
}

void SessionHandler::set_session(const std::string &session_id, const std::string &new_value)
{
	_session[session_id] = new_value;
}