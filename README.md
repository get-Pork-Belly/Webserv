<!--
*** Thanks for checking out the Best-README-Template. If you have a suggestion
*** that would make this better, please fork the repo and create a pull request
*** or simply open an issue with the tag "enhancement".
*** Thanks again! Now go create something AMAZING! :D
***
***
***
*** To avoid retyping too much info. Do a search and replace for the following:
*** github_username, repo_name, twitter_handle, email, project_title, project_description
-->



<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]



<!-- PROJECT LOGO -->
<br />
<p align="center">
  <a href="https://github.com/get-Pork-Belly/Webserv">
    <img src="https://user-images.githubusercontent.com/49181231/112866996-5f963980-90f5-11eb-80ae-5ec7c2051033.png" alt="Logo" width="80" height="80">
  </a>

  <h3 align="center">Webserv</h3>

  <p align="center">
    이 프로젝트는 Customized 한 HTTP 서버를 만드는 과제입니다. 해당 과제를 수행하기 위해 RFC 문서 중 HTTP에 해당하는 파트(7230-7235)를 숙지하였으며 실제 브라우저에서 테스트할 수 있도록 TCP/IP 통신을 위한 소켓을 C++로 구현하였습니다.
    <br />

  </p>
</p>



<!-- TABLE OF CONTENTS -->

<summary><h2 style="display: inline-block">Table of Contents</h2></summary>
<details open="open">
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
    </li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#acknowledgements">Wiki</a></li>
    <li><a href="#contact">Contact</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

![image](https://user-images.githubusercontent.com/49181231/112867764-40e47280-90f6-11eb-8b9b-eaf2de99cbb4.png)



<!-- GETTING STARTED -->
## Getting Started

1. Clone the repo
   ```sh
   git clone https://github.com/get-Pork-Belly/Webserv
   ```
2. Make and Edit config file
   ```sh
   cp config/"iwoo or sanam or yohlee _config" config/"YOUR_CONFIG_FILE_NAME"
   ```
   ```sh
   # edit your server listening port & server name ...
   ```
3. Make & Run
   ```sh
   make && ./Webserv "CONFIG_FILE_PATH"
   ```


<!-- ROADMAP -->
## Roadmap

- [Project DashBoard](https://github.com/get-Pork-Belly/Webserv/projects/1)
- [Open Issues](https://github.com/get-Pork-Belly/Webserv/issues)


<!-- Wiki -->
## Wiki

* [CGI(Common Gateway Interface)](https://github.com/get-Pork-Belly/Webserv/wiki/CGI(Common-Gateway-Interface))
* [CGI와 파이프 그리고 dup](https://github.com/get-Pork-Belly/Webserv/wiki/CGI%EC%99%80-%ED%8C%8C%EC%9D%B4%ED%94%84-%EA%B7%B8%EB%A6%AC%EA%B3%A0-dup)
* [HTTP 상태코드](https://github.com/get-Pork-Belly/Webserv/wiki/HTTP-%EC%83%81%ED%83%9C%EC%BD%94%EB%93%9C)
* [HTTP 헤더](https://github.com/get-Pork-Belly/Webserv/wiki/HTTP-%ED%97%A4%EB%8D%94)
* [Nginx Directives](https://github.com/get-Pork-Belly/Webserv/wiki/Nginx---Directives)
* [Nginx beginner](https://github.com/get-Pork-Belly/Webserv/wiki/Nginx-beginner)
* [Request 정리](https://github.com/get-Pork-Belly/Webserv/wiki/Request-%EC%A0%95%EB%A6%AC)
* [Response 정리](https://github.com/get-Pork-Belly/Webserv/wiki/Response-%EC%A0%95%EB%A6%AC)
* [Select와 usleep (feat.Pipe의 IO Stream)](https://github.com/get-Pork-Belly/Webserv/wiki/Select%EC%99%80-usleep-(feat.-Pipe%EC%9D%98-I-O-Stream))
* [URI 정리](https://github.com/get-Pork-Belly/Webserv/wiki/URI-%EC%A0%95%EB%A6%AC)
* [소켓프로그래밍](https://github.com/get-Pork-Belly/Webserv/wiki/%EC%86%8C%EC%BC%93%ED%94%84%EB%A1%9C%EA%B7%B8%EB%9E%98%EB%B0%8D)
* [콘텐츠 인코딩, 청크 인코딩](https://github.com/get-Pork-Belly/Webserv/wiki/%EC%BD%98%ED%85%90%EC%B8%A0-%EC%9D%B8%EC%BD%94%EB%94%A9,-%EC%B2%AD%ED%81%AC-%EC%9D%B8%EC%BD%94%EB%94%A9)
* [협업규칙](https://github.com/get-Pork-Belly/Webserv/wiki/%ED%98%91%EC%97%85%EA%B7%9C%EC%B9%99)
* [Class 명세서](https://docs.google.com/spreadsheets/d/1GNUoCfloWzp0r27ZsayIxHRwFWTl2l4ZfWlXcVARf98/edit#gid=0)


<!-- CONTACT -->
## Contact

* [Iwoo](https://github.com/humblEgo)
* [Sanam](https://github.com/simian114)
* [Yohlee](https://github.com/l-yohai)



<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/get-Pork-Belly/Webserv?style=for-the-badge
[contributors-url]: https://github.com/get-Pork-Belly/Webserv/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/get-Pork-Belly/Webserv?style=for-the-badge
[forks-url]: https://github.com/get-Pork-Belly/Webserv/network/members
[stars-shield]: https://img.shields.io/github/stars/get-Pork-Belly/Webserv?style=for-the-badge
[stars-url]: https://github.com/get-Pork-Belly/Webserv/stargazers
[issues-shield]: https://img.shields.io/github/issues/get-Pork-Belly/Webserv?style=for-the-badge
[issues-url]: https://github.com/get-Pork-Belly/Webserv/issues
