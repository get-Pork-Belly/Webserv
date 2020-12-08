def python_cgi(s):
	result = ''
	result += 'Status: 200 OK\r\n';
	result += 'Content-Type: text/html; charset=utf-8\r\n\r\n';
	
	result += '<html>\n\t<head>\n\t\t<title>GPB\'s Webserv</title>\n\t</head>\n\t'
	result += '<body>\n\t\t<center>\n\t\t\t<h1>Please enter a valid python script path.'
	result += '</h1>\n\t\t</center>\n\t\t<hr>\n\t<center> GPB\'s Webserv </center>\n\t</body>\n</html>'
	return result