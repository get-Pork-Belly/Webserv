def python_cgi(s):
	result = ''
	result += 'Status: 200 OK\r\n';
	result += 'Content-Type: text/html; charset=utf-8\r\n\r\n';
	result += s.lower()
	return result