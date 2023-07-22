#include "SessionHandler.hpp"
#include "Util.hpp"
#include <cstdlib>

void SessionHandler::execute(Request &req, Response &res)
{
	_current_session_ID = extract_cookie(req.get_header("Cookie"), SESSION_COOKIE_NAME);
	if (_current_session_ID.empty())
	{
		_current_session_ID = Util::to_string(rand());
		res.set_header("Set-Cookie", SESSION_COOKIE_NAME "=" + _current_session_ID);
	}
	Middleware::execute(req, res);
}

std::string SessionHandler::get_session() const
{
	try
	{
		return _sessions.at(_current_session_ID);
	}
	catch (...)
	{
		return std::string();
	}
}

const std::string &SessionHandler::get_session_id() const
{
	return _current_session_ID;
}

std::string SessionHandler::extract_cookie(const std::string &cookies, const std::string &cookie_name)
{
	std::size_t begin = cookies.find(cookie_name + "=");
	if (begin == std::string::npos)
		return std::string();
	begin += cookie_name.size() + 1;
	const std::size_t end = cookies.find(';', begin);
	if (end == std::string::npos)
		return cookies.substr(begin);
	else
		return cookies.substr(begin, end - begin);
}

void SessionHandler::set_session(const std::string &session_id, const std::string &new_value)
{
	_sessions[session_id] = new_value;
}

bool SessionHandler::parse_session(const std::string &session_id, const std::string &cgi_response_header)
{
	const char 		session_key[] = "X-Replace-Session: ";
	const size_t	len = sizeof(session_key) - 1; // exclude null-terminator

	if (cgi_response_header.compare(0, len, session_key) == 0)
	{
		set_session(session_id, cgi_response_header.substr(len));
		return true;
	}
	return false;
}
