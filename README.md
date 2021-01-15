# Webserv

implement nginx

목표: config파일에 default error page .html 경로를 설정해주면, 해당 에러는 default error page가 보인다.

1. config file에 아래처럼 지시자를 적는다. (server, http, location 블록 중 어디에 넣을지 생각하긴 해야함.) -> http, server 블록 둘에 적으면 될거같음
   default_error_page: ./www/default_error.html;
2. ServerGenerator에서 에러페이지 경로에 있는 파일을 읽어와서 만약 그 파일이 존재한다면, Response의 멤버변수에 담아준다.
   -> 에러 넘버를 키로 하는 맵으로 저장 server의 멤버 변수 -> errors
3. PageGenerator에서 에러페이지를 생성할 때, error_page가 설정되어있는지 확인하고, -> 현재 pageGenerator 의 인자로 res 하나만 넘어가고 있음. server의 멤버 변수 errors 를 넘겨주자
   1. 설정 되어있다면 서버에 저장되어있는 변수를 참조해서 에러페이지를 생성하고,
   2. 설정 되어있지 않다면, 기존 방식대로 에러페이지를 생성한다.

// default_error_page ./www/errors/html;
body += "<html>\n\t<head>\n\t\t<title>" + status_code + " " + message +
"</title>\n\t</head>" + "\n\t<body>\n\t\t<center>\n\t\t\t<h1>" +
status_code + " " + message + "</h1>\n\t\t</center>" +
"\n\t\t<hr>\n\t<center> ft_nginx </center>\n\t</body>\n</html>";
