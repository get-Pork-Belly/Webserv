#include "Request.hpp"

void printRecvRequest(std::string str, Request& req)
{
    std::string tmp(str);

    std::cout<<tmp<<std::endl;
    if (req.getRecvRequest() == RecvRequest::REQUEST_LINE)
        std::cout<<"RecvRequest: READY"<<std::endl;
    else if (req.getRecvRequest() == RecvRequest::COMPLETE)
        std::cout<<"RecvRequest: COMPLETE"<<std::endl;
    else if (req.getRecvRequest() == RecvRequest::NORMAL_BODY)
        std::cout<<"RecvRequest: NORMAL_BODY"<<std::endl;
    else if (req.getRecvRequest() == RecvRequest::CHUNKED_BODY)
        std::cout<<"RecvRequest: CHUNKED_BODY"<<std::endl;
}

int main()
{
    Request req_ready;
    Request req_completed;
    Request req_normal;
    Request req_chunked;

    req_completed.setRecvRequest(RecvRequest::COMPLETE);
    req_normal.setRecvRequest(RecvRequest::NORMAL_BODY);
    req_chunked.setRecvRequest(RecvRequest::CHUNKED_BODY);

    req_ready.updateRecvRequest();
    std::cout<<"Not yet requested"<<std::endl;
    printRecvRequest("", req_ready);
    req_ready.setMethod(std::string("PUT"));
    req_ready.setHeaders("Transfer-Encoding", "chunked");
    std::cout<<"Now method is "<<req_ready.getMethod()<<std::endl;
    for (auto& kv : req_ready.getHeaders())
	{
		std::cout << "Key: " << kv.first << " value: " << kv.second << std::endl;
	}
    printRecvRequest("before update", req_ready); //READY
    req_ready.updateRecvRequest();
    printRecvRequest("after update", req_ready); //NORMAL_BODY

    return (0);
}